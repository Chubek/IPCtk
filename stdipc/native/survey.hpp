 #pragma once
 
 #include <ipctk.hpp>
 #include <string>
 
 namespace ipctk::stdipc {
 
 inline auto build_survey(const std::string& multicast_addr,
                           int multicast_port,
                           const std::string& response_addr,
                           int response_port,
                           int deadline_ms) {
   using namespace ipctk::dsl;
 
   auto survey_out  = socket("survey_out")  = udp.multicast(multicast_addr + ":" + std::to_string(multicast_port));
   auto response_in = socket("response_in") = tcp.listen(response_addr + ":" + std::to_string(response_port));
 
   auto respondent_table = shared("respondent_table") = shm.open("/respondents", 128_KiB);
   auto respondent_lock  = mutex("respondent_lock")   = semaphore("/respondents.lock", 1);
   auto survey_deadline  = timer("survey_deadline")   = timer(std::to_string(deadline_ms) + "_ms");
 
   auto survey_path =
     pipe("survey_path") =
       recv(survey_out)
       >> decode(as<question>)
       >> broadcast(udp.multicast(multicast_addr + ":" + std::to_string(multicast_port)))
       >> start(survey_deadline);
 
   auto respond_path =
     pipe("respond_path") =
       recv(response_in)
       >> decode(as<answer>)
       >> lock(respondent_lock)
       >> insert(respondent_table)
       >> unlock(respondent_lock);
 
   auto collect_path =
     pipe("collect_path") =
       expire(survey_deadline)
       >> lock(respondent_lock)
       >> aggregate(respondent_table)
       >> clear(respondent_table)
       >> unlock(respondent_lock)
       >> encode(as<results>)
       >> send(survey_out);
 
   return std::make_tuple(survey_out, response_in, respondent_table, respondent_lock,
                          survey_deadline, survey_path, respond_path, collect_path);
 }
 
 } // namespace ipctk::stdipc
